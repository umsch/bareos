<template>
  <q-item clickable tag="a" :href="link">
    <q-item-section v-if="icon" avatar>
      <q-icon :name="icon" />
    </q-item-section>

    <q-item-section>
      <q-item-label>{{ $t(title) }}</q-item-label>
      <q-item-label v-if="caption" caption>{{ $t(caption) }}</q-item-label>
    </q-item-section>
  </q-item>
</template>

<script setup lang="ts">
defineOptions({
  name: 'EssentialLink',
})

export interface EssentialLinkProps {
  title: string
  caption?: string
  link?: string
  icon?: string
}

withDefaults(defineProps<EssentialLinkProps>(), {
  caption: '',
  link: '#',
  icon: '',
})
</script>
